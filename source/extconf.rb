require "mkmf"

SYSTEM_LIBRARIES = [
  "dsound",
  "ole32",
]

SYSTEM_LIBRARIES.each do |lib|
  have_library(lib)
end

#�w�b�_�t�@�C������Ă܂��񂪒��ׂ�̖ʓ|��(^-^;

#have_func("rb_enc_str_new") 

create_makefile("ayame")
