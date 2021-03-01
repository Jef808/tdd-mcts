;;; Directory Local Variables
;;; For more information see (info "(emacs) Directory Variables")

((nil . ((projectile-project-name . "TDD-MCTS")
         (projectile-project-configure-cmd . "cmake -B ../build && cp compile_commands.json ../")
         ;;(projectile-project-compilation-dir . "./build/")
         (projectile-project-compilation-cmd . "make-k all")
         (projectile-project-test-cmd . "./testTictactoe")))
 (c++-mode . ((c-file-style . "user")))
 (lisp-data-mode . ((eval . (rainbow-delimiters-mode)))))
