(setq draw (lambda ()
  (progn
    (background 0 0 0)
    (rotate-x (lerp 1 -1 (mouse-y)))
    (rotate-y (lerp 1 -1 (mouse-x)))
    (tetrahedron)))))
