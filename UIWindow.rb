module UIWindow
    @@debug = true
    
    UI_color = [255, 0, 0, 80]
    UI_Side_width = 230
    UI = {:side=>{
            :left=>{
                :x=>0,:y=>0,:image=>Image.new( UI_Side_width, Window.height, UI_color)
            },
            :right=>{
                :x=>Window.height - UI_Side_width, :y=>0, :image=>Image.new( UI_Side_width, Window.height, UI_color)
            }
        },
        :under=>{
             :x=>0, :y=>Window.height - Fonts::Large.size, :image=>Image.new( Window.width, Fonts::Large.size, UI_color)
        }
    }
    
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
    
    def self.draw()
        
    end
    
    def self.draw_font()
    end
    
    def self.draw_ui()
        Window.draw( UI[:side][:left][:x], UI[:side][:left][:y], UI[:side][:left][:image])
        Window.draw( UI[:side][:right][:x], UI[:side][:right][:y], UI[:side][:right][:image])
        Window.draw( UI[:under][:x], UI[:under][:y], UI[:under][:image])
    end
end