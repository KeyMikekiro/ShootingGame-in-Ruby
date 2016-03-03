module UIWindow
    @@debug = true
    
    def self.draw_start_title()
        title_font = Fonts::Middle
        start_title = "Welcome to Test Ruby Game."
        Window.draw_font( (Window.width / 2 - title_font.get_width(start_title) / 2), 
            (Window.height / 2 - title_font.size / 2), start_title, title_font )
    end
    
    def self.debug=(enable)
        @@debug = enable
    end
    
    def self.debug_draw( x, y, image, z=0)
        Window.draw( x, y, image, z) if @@debug
    end
    
    def self.debug_draw_font( x, y, string, font, option={})
        Window.draw_font( x, y, string, font, option) if @@debug
    end
    
    def self.debug?()
        return @@debug
    end
end