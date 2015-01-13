(let (nl (str [0a]))
  (def minilisp-major-version 0)
  (def minilisp-minor-version 5)
  (def minilisp-patch-version 0)

  (def is-nil  (fn (x) (= 0 (type x))))
  (def exists  (fn (x) (> (type x) 0)))
  (def is-int  (fn (x) (= 1 (type x))))
  (def is-list (fn (x) (= 2 (type x))))
  (def is-str  (fn (x) (= 7 (type x))))
  
  (def caar (fn (x) (car (car x))))
  (def cadr (fn (x) (car (cdr x))))
  (def cdar (fn (x) (cdr (car x))))
  (def cadar (fn (x) (car (cdr (car x)))))

  (def last (fn (l) (if (exists (cdr l)) (last (cdr l)) l )))
  (def llen (fn (l) (if (exists (car l)) (+ 1 (llen (cdr l))) 0)))

  (def str->list (fn (s) (map (fn (c) c) s)))

  (def string-positions (fn (s needle)
                            (filter (fn (x) (> x -1))
                                    (map (fn (c idx) (if (= c needle) idx -1)) (str->list s)))))

  (def split (fn (s sep)
                 (let (poses (string-positions s sep))
                   (if (is-int (car poses))
                       (if (< (car poses) 1)
                           (split (substr s (+ 1 (car poses))) sep)
                         (cons (substr s 0 (car poses))
                               (split (substr s (+ 1 (car poses))) sep)))
                     (list s))
                   )))

  (def url-decode (fn (s)
                      (let (poses (string-positions s "%"))
                        (if (is-int (car poses))
                            (concat (list
                             (substr s 0 (car poses))
                             (str (read (concat "[" (substr s (+ 1 (car poses)) 2) "]")))
                             (url-decode (substr s (+ 3 (car poses))))))
                          s))))
  
  (def addrbook-path "addrs.l")

  (def load-addrs! (fn ()
                      (def addrs (read (str (load addrbook-path))))))

  (def addr-name (fn (row) (cadar row)))
  
  (def get-addr (fn (id) (filter (fn (row) (= (addr-name row) id)) addrs)))

  (def add-addr! (fn (row) (def addrs (cons (cons (+ 1 (llen addrs)) row) addrs))))


  (def save-addrs (fn ()
                      (save addrbook-path (concat addrs))))


  
  (def nl nl)
  
  (def response-header (concat "HTTP/1.1 200 OK" nl "Connection: close" nl "Content-Type: text/html; charset=utf-8" nl nl))

  (def addrs-html
       (fn () (concat (map
                       (fn (row) (concat "<li><a href='/addrs/" (addr-name row) "'>" (addr-name row) "</a> " "</li>"))
                       addrs))))

  (def addr-html
       (fn (a) (concat
                "<table>" (concat (map
                                   (fn (row) (concat "<tr><td>" (car row) "</td><td>"
                                                     (if (= (car row) "phone")
                                                         (concat "<a href='tel:" (cadr row) "'>" (cadr row) "</a>")
                                                       (cadr row))
                                                     "</td></tr>" nl))
                                   (cdr a))) "</table>")))

  (def httpd-post
       (fn (path body) (let (path (split path "/")
                                  resource (car path)
                                  id (car (cdr path))
                                  kvs (split body "&"))

                         (add-addr! (map (fn (kv) (split kv "=")) kvs))
                         
                         (concat response-header
                                 "<html>POST body " (len body) body "</html>"))))
  
  (def httpd-get
       (fn (path) (let (path (split path "/") resource (car path) id (url-decode (car (cdr path))))
                    (if (= "reload" resource) (load-addrs!))

                    (concat response-header
                            "<html>"
                            
                            (if (= "addrs" resource)
                                (if (is-str id)
                                    (concat "<h1>" id "</h1>" (addr-html (car (get-addr id))))
                                  (concat "<h1>Address Book</h1>" nl "<ul>" (addrs-html) "</ul>" nl))
                              "<h1>nothing here.</h1>")

                            "<p><a href='/addrs'>all addrs</a> | <a href='/reload'>reload book</a></p>"
                            "<form action='/addrs' method='post'>"
                            "Name: <input type='text' name='name'>"
                            "Phone: <input type='text' name='phone'>"
                            "<input type='submit' value='add'>"
                            "</form>" 
                            "</html>" nl)
                    ) ))
)
