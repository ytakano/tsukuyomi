#include "MCJITHelper.hpp"

#include <llvm/Transforms/Scalar/GVN.h>

void *HelpingMemoryManager::getPointerToNamedFunction(const std::string &Name,
                                                      bool AbortOnFailure)
{
    // Try the standard symbol resolution first, but ask it not to abort.
    void *pfn = SectionMemoryManager::getPointerToNamedFunction(Name, false);
    if (pfn)
        return pfn;

    pfn = MasterHelper->getPointerToNamedFunction(Name);
    if (!pfn && AbortOnFailure)
        llvm::report_fatal_error("Program used external function '" + Name +
                                 "' which could not be resolved!");
    return pfn;
}

MCJITHelper::~MCJITHelper()
{
    // Walk the vector of modules.
    ModuleVector::iterator it, end;
    for (it = Modules.begin(), end = Modules.end(); it != end; ++it) {
        // See if we have an execution engine for this module.
        auto mapIt = EngineMap.find(*it);
        // If we have an EE, the EE owns the module so just delete the EE.
        if (mapIt != EngineMap.end()) {
            delete mapIt->second;
        } else {
            // Otherwise, we still own the module.  Delete it now.
            delete *it;
        }
    }
}

llvm::Function*
MCJITHelper::getFunction(const std::string FnName)
{
    ModuleVector::iterator begin = Modules.begin();
    ModuleVector::iterator end   = Modules.end();
    ModuleVector::iterator it;
    for (it = begin; it != end; ++it) {
        llvm::Function *F = (*it)->getFunction(FnName);
        if (F) {
            if (*it == OpenModule)
                return F;

            assert(OpenModule != NULL);

            // This function is in a module that has already been JITed.
            // We need to generate a new prototype for external linkage.
            llvm::Function *PF = OpenModule->getFunction(FnName);
            if (PF && !PF->empty()) {
                PRINTERR("redefinition of function across modules");
                return 0;
            }

            // If we don't have a prototype yet, create one.
            if (!PF)
                PF = llvm::Function::Create(F->getFunctionType(),
                                            llvm::Function::ExternalLinkage,
                                            FnName,
                                            OpenModule);
            return PF;
        }
    }
    return NULL;
}

llvm::Module*
MCJITHelper::getModuleForNewFunction(std::string &FnName)
{
    // If we have a Module that hasn't been JITed, use that.
    if (OpenModule)
        return OpenModule;

    // Otherwise create a new Module.
    std::string ModName("module_" + FnName);
    llvm::Module *M = new llvm::Module(ModName, Context);
    Modules.push_back(M);
    OpenModule = M;
    return M;
}

void*
MCJITHelper::getPointerToFunction(llvm::Function* F)
{
    // Look for this function in an existing module
    ModuleVector::iterator begin = Modules.begin();
    ModuleVector::iterator end   = Modules.end();
    ModuleVector::iterator it;
    std::string FnName = F->getName();
    for (it = begin; it != end; ++it) {
        llvm::Function *MF = (*it)->getFunction(FnName);
        if (MF == F) {
            auto eeIt = EngineMap.find(*it);
            if (eeIt != EngineMap.end()) {
                void *P = eeIt->second->getPointerToFunction(F);
                if (P)
                    return P;
            } else {
                llvm::ExecutionEngine *EE = compileModule(*it);
                void *P = EE->getPointerToFunction(F);
                if (P)
                    return P;
            }
        }
    }
    return NULL;
}

void
MCJITHelper::closeCurrentModule()
{
    OpenModule = NULL;
}

llvm::ExecutionEngine*
MCJITHelper::compileModule(llvm::Module *M)
{
    if (M == OpenModule)
        closeCurrentModule();

    std::string ErrStr;
    llvm::ExecutionEngine *NewEngine =
        llvm::EngineBuilder(std::unique_ptr<llvm::Module>(M))
            .setErrorStr(&ErrStr)
            .setMCJITMemoryManager(llvm::make_unique<HelpingMemoryManager>(this))
            .create();
    if (!NewEngine) {
        PRINTERR("Could not create ExecutionEngine: %s\n", ErrStr.c_str());
        exit(1);
    }

    // Create a function pass manager for this engine
    llvm::legacy::FunctionPassManager *FPM = new llvm::legacy::FunctionPassManager(M);

    // Set up the optimizer pipeline.  Start with registering info about how the
    // target lays out data structures.
#if LLVM_VERSION_MAJOR == 3 && LLVM_VERSION_MINOR == 7
    M->setDataLayout(*NewEngine->getDataLayout());
#else
    M->setDataLayout(NewEngine->getDataLayout());
#endif // LLVM_VERSION_MAJOR == 3 && LLVM_VERSION_MINOR == 7

    //FPM->add(new llvm::DataLayout(*NewEngine->getDataLayout()));
    // Provide basic AliasAnalysis support for GVN.
    FPM->add(llvm::createBasicAAWrapperPass());
    // Promote allocas to registers.
    FPM->add(llvm::createPromoteMemoryToRegisterPass());
    // Do simple "peephole" optimizations and bit-twiddling optzns.
    FPM->add(llvm::createInstructionCombiningPass());
    // Reassociate expressions.
    FPM->add(llvm::createReassociatePass());
    // Eliminate Common SubExpressions.
    FPM->add(llvm::createGVNPass());
    // Simplify the control flow graph (deleting unreachable blocks, etc).
    FPM->add(llvm::createCFGSimplificationPass());
    FPM->doInitialization();

    // For each function in the module
    llvm::Module::iterator it;
    llvm::Module::iterator end = M->end();
    for (it = M->begin(); it != end; ++it) {
        // Run the FPM on this function
        FPM->run(*it);
    }

    // We don't need this anymore
    delete FPM;

    // Store this engine
    EngineMap[M] = NewEngine;
    NewEngine->finalizeObject();

    return NewEngine;
}

void
*MCJITHelper::getPointerToNamedFunction(const std::string &Name)
{
    // Look for the functions in our modules, compiling only as necessary
    ModuleVector::iterator begin = Modules.begin();
    ModuleVector::iterator end   = Modules.end();
    ModuleVector::iterator it;
    for (it = begin; it != end; ++it) {
        llvm::Function *F = (*it)->getFunction(Name);
        if (F && !F->empty()) {
            auto eeIt = EngineMap.find(*it);
            if (eeIt != EngineMap.end()) {
                void *P = eeIt->second->getPointerToFunction(F);
                if (P)
                    return P;
            } else {
                llvm::ExecutionEngine *EE = compileModule(*it);
                void *P = EE->getPointerToFunction(F);
                if (P)
                    return P;
            }
        }
    }
    return NULL;
}

void
MCJITHelper::dump()
{
    ModuleVector::iterator begin = Modules.begin();
    ModuleVector::iterator end   = Modules.end();
    ModuleVector::iterator it;
    for (it = begin; it != end; ++it)
        (*it)->print(llvm::errs(), nullptr);
}
