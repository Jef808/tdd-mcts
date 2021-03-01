;;; Directory Local Variables
;;; For more information see (info "(emacs) Directory Variables")

((nil . ((projectile-project-name . "TDD-MCTS")
         (projectile-project-configure-cmd . "cmake -B ../build && cp compile_commands.json ../")
         (projectile-project-compilation-cmd . "make-k all")
         (projectile-project-test-cmd . "./testState")))

 (c++-mode . ((c-file-style . "user")
              (c-offsets-alist . ((substatement-open . 0)))))

 (lisp-data-mode . ((eval . (rainbow-delimiters-mode)))))
