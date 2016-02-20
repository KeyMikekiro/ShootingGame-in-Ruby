module Fonts
    FontArray = Font.install( "./fonts/MioW4.otf")
    Large = Font.new(32, FontArray[0])
    Middle = Font.new(24, FontArray[0])
    Small = Font.new(16, FontArray[0])
    Tiny = Font.new(8, FontArray[0])
end