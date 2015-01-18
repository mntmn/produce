(let ()
  (def lookup (fn (a-list needle) (car (cdr (car (filter (fn (x) (= (car x) needle)) a-list))))))

  (def fresh-track-id (fn () (+ 1 (len (all-tracks)))))

  (def notes (quote
              (("c2" 36)
               ("c#2" 37)
               ("d2" 38)
               ("d#2" 39)
               ("e2" 40)
               ("f2" 41)
               ("f#2" 42)
               ("g2" 43)
               ("g#2" 44)
               ("a2" 45)
               ("a#2" 46)
               ("b2" 47)

               ("c3" 48)
               ("c#3" 49)
               ("d3" 50)
               ("d#3" 51)
               ("e3" 52)
               ("f3" 53)
               ("f#3" 54)
               ("g3" 55)
               ("g#3" 56)
               ("a3" 57)
               ("a#3" 58)
               ("b3" 59)

               ("c4" 60)
               ("c#4" 61)
               ("d4" 62)
               ("d#4" 63)
               ("e4" 64)
               ("f4" 65)
               ("f#4" 66)
               ("g4" 67)
               ("g#4" 68)
               ("a4" 69)
               ("a#4" 70)
               ("b4" 71))))

  (def note (fn (s) (lookup notes s)))
  
  (def project-load (fn (path) (let (prj (read (str (load path))))
                                 (print "loaded: " prj)
                                 (eval prj))))
  
  (def project-save (fn (path) (save path (str (append
                                                (quote (let (project-version 1)))
                                                (reverse (all-instruments)) (reverse (all-tracks)) (reverse (all-regions)))))))

  (def key-bindings (quote                 
                     (("l" (project-load "project2.l"))

                     ("x" (project-load "project.l"))
                      
                     ("s" (project-save "project2.l"))

                     ("2" (set-regions-length 250))
                     
                     ("t" (let (tid fresh-track-id)
                            (track tid "untitled track")))

                     ("a" (add-region-at-mouse))
                     )))
  
  (def handle-key (fn (sym) (let ()
                              (print (list "handle-key" sym))
                              (eval (lookup key-bindings sym)))))
  
  (print "end of init.l")
)
