(let ()
  (def lookup (fn (a-list needle) (car (cdr (car (filter (fn (x) (= (car x) needle)) a-list))))))

  (def fresh-track-id (fn () (+ 1 (len (all-tracks)))))
  
  (def project-load (fn (path) (let (prj (read (str (load path))))
                                 (print "loaded: " prj)
                                 (project-clear)
                                 (eval prj))))
  
  (def key-bindings (quote (               
                     ("l" (project-load (project-path)))
                      
                     ("s" (project-save (project-path)))

                     ("1" (set-regions-length 125))
                     ("2" (set-regions-length 250))
                     ("3" (set-regions-length 500))
                     ("4" (set-regions-length 750))
                     
                     ("t" (let (tid fresh-track-id)
                            (track tid "untitled track")))

                     ("d" (delete-selected-tracks))

                     ("b" (bounce-loop))

                     ("a" (add-region-at-mouse))
                     ("e" (edit-region-external))
                     ("(" (interactive-eval))
                     )))
  
  (def handle-key (fn (sym) (let ()
                              (print (list "handle-key" sym))
                              (eval (lookup key-bindings sym)))))
  
  (print "end of init.l")
)
