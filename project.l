(let ()
(instrument 0 "wave('/media/storage/samples/clean/pend_kick-x-01.wav') * 0.8")
(instrument 1 "wave('/media/storage/samples/clean/pend_sn-x-04.wav') * 0.5")
(instrument 2 "wave('/media/storage/samples/clean/pend_hats-x-01.wav')")
(instrument 3 "sin(hz=130.0) * adsr(0.12, 0.12, 0.7, 0.30) * 40.0")
(instrument 4 "sin(hz=260.0) * adsr(0.12, 0.12, 0.7, 0.30) * 40.0")

(track 0 "kicks")
(track 1 "snares")
(track 2 "hats")
(track 3 "beep")
(track 4 "beep")


(region 0 0 0  0   50)
(region 1 0 0  500 50)

(region 2 1 1  250 50)
(region 3 1 1  750 50)
(region 4 1 1  900 50)

(region 5 3 3  250 100)
(region 5 3 3  500 100)
(region 6 4 4  250 100)

)

