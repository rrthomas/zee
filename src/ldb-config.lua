return {
  output = function (...)
             local arg = list.map (tostring, {...})
             io.flush ()
             io.write (unpack (arg))
             io.flush ()
           end
}
