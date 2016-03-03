module UIWindow
    @@debug = true
    
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