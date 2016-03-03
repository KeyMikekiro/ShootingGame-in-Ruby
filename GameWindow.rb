module GameWindow
    @@debug = true
    
    UI_draw_order = 500
    UI_draw_font_order = 1200
    Debug_draw_order = 500
    Debug_draw_font_order = 1200
    
    Game_Draw_order = 200
    Game_Draw_Font_order = 1000
    
    UI_color = [255, 0, 0, 80]
    UI_Side_width = 230
    UI = {:side=>{
            :left=>{
                :x=>0,:y=>0,:image=>Image.new( UI_Side_width, Window.height, UI_color)
            },
            :right=>{
                :x=>Window.width - UI_Side_width, :y=>0, :image=>Image.new( UI_Side_width, Window.height, UI_color)
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
        Window.draw( x, y, image, Debug_draw_font_order) if @@debug && z<=0
        Window.draw( x, y, image, z) if @@debug && z>0
    end
    
    def self.debug_draw_font( x, y, string, font, option={})
        option.store(:z,Debug_draw_font_order) if option[:z] == nil
        Window.draw_font( x, y, string, font, option) if @@debug
    end
    
    def self.debug?()
        return @@debug
    end
    
    def self.draw( x, y, image, z=0)
        Window.draw( x, y, image, Game_Draw_order) if z<=0
        Window.draw( x, y, image, z) if z>0
    end
    
    def self.draw_font( x, y, string, font, option={})
        option.store(:z,Game_Draw_Font_order) if option[:z] == nil
        Window.draw_font( x, y, string, font, option)
    end
    
    def self.x()
        return UI[:side][:left][:x] + UI[:side][:left][:image].width
    end
    
    def self.width()
        return UI[:side][:right][:x]
    end
    
    def self.y()
        return 0
    end
    
    def self.height()
        return UI[:under][:y]
    end
    
    def self.draw_ui()
        Window.draw( UI[:side][:left][:x], UI[:side][:left][:y], UI[:side][:left][:image], UI_draw_order)
        Window.draw( UI[:side][:right][:x], UI[:side][:right][:y], UI[:side][:right][:image], UI_draw_order)
        Window.draw( UI[:under][:x], UI[:under][:y], UI[:under][:image], UI_draw_order)
    end
end