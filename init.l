(let ()
  (def lookup (fn (a-list needle) (car (cdr (car (filter (fn (x) (= (car x) needle)) a-list))))))

  (def fresh-track-id (fn () (+ 1 (len (all-tracks)))))
  
  (def project-load (fn (path) (let (prj (read (str (load path))))
                                 (print "loaded: " prj)
                                 (eval prj))))
  
  (def project-save (fn (path) (save path (str (append
                                                (quote (let (project-version 1)))
                                                (reverse (all-instruments)) (reverse (all-tracks)) (reverse (all-regions)))))))

  (def key-bindings (quote                 
                     (("l" (project-load "project2.l"))

                     ("s" (project-save "project2.l"))
                     
                     ("t" (let (tid fresh-track-id)
                                        (track tid "untitled track")))
                     ("x" (print "foobar!"))
                     )))
  
  (def handle-key (fn (sym) (let ()
                              (print (list "handle-key" sym))
                              (eval (lookup key-bindings sym)))))
  
  (print "end of init.l")
)
