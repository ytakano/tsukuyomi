(define checkbook (lambda ()
        (letrec

            ; These strings are used as prompts

           ((IB "Enter initial balance: ")
            (AT "Enter transaction (- for withdrawal): ")
            (FB "Your final balance is: ")

            ; This function displays a prompt then returns
            ; a value read.

            (prompt-read (lambda (Prompt)

                  (display Prompt)
                  (read)))

            ; This function recursively computes the new
            ; balance given an initial balance init and
            ; a new value t.  Termination occurs when the
            ; new value is 0.

            (newbal (lambda (Init t)
                  (if (= t 0)
                      (list FB Init)
                      (transaction (+ Init t)))))

            ; This function prompts for and reads the next
            ; transaction and passes the information to newbal

            (transaction (lambda (Init)
                      (newbal Init (prompt-read AT)))))

; This is the body of checkbook;  it prompts for the
; starting balance

  (transaction (prompt-read IB)))))
