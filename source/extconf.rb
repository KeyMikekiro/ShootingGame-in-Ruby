require "mkmf"

SYSTEM_LIBRARIES = [
  "dsound",
  "ole32",
]

SYSTEM_LIBRARIES.each do |lib|
  have_library(lib)
end

#ヘッダファイル足りてませんが調べるの面倒で(^-^;

#have_func("rb_enc_str_new") 

create_makefile("ayame")
