module GameWindow
    @@debug = true
    @@pause = false
    
    UI_draw_order = 500
    UI_draw_font_order = 1200
    Debug_draw_order = 500
    Debug_draw_font_order = 1200
    
    Game_draw_order = 200
    Game_draw_font_order = 1000
    UI_Font = Fonts::Middle
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
        },
        :pause=>{
            :x=>Window.width / 2 - UI_Font.get_width("pause Restart to [ESC].") / 2, :y=>Window.height / 2, :string=>"pause Restart to [ESC]."
        },
        :gameover=>{
            :x=>Window.width / 2 - UI_Font.get_width("Game Over") / 2, :y=>Window.height / 2, :string=>"Game Over"
        }
    }
    
    def self.draw_start_title()
        title_font = UI_Font
        start_title = "Welcome to Test Ruby Game."
        Window.draw_font( (Window.width / 2 - title_font.get_width(start_title) / 2), 
            (Window.height / 2 - title_font.size / 2), start_title, title_font )
    end
    
    def self.chengePause()
        @@pause = !@@pause
    end
    
    def self.pause?()
        return @@pause
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
        Window.draw( x, y, image, Game_draw_order) if z<=0
        Window.draw( x, y, image, z) if z>0
    end
    
    def self.draw_sprite( sprite,z=0)
        draw( sprite.x, sprite.y, sprite.image) if z <= 0
        draw( sprite.x, sprite.y, sprite.image, z) if z > 0
    end
    
    def self.draw_font( x, y, string, font, option={})
        option.store(:z,Game_draw_font_order) if option[:z] == nil
        Window.draw_font( x, y, string, font, option)
    end
    
    def self.x()
        return UI[:side][:left][:x] + UI[:side][:left][:image].width
    end
    
    def self.center_horizontal
        return Window.width / 2
    end
    
    def self.width()
        return UI[:side][:right][:x]
    end
    
    def self.y()
        return 0
    end
    
    def self.center_vertical
        return Window.height / 2
    end
    
    
    def self.height()
        return UI[:under][:y]
    end
    
    def self.draw_gameover()
        Window.draw_font( UI[:gameover][:x], UI[:gameover][:y], UI[:gameover][:string], Fonts::Large, option={:z=>UI_draw_font_order})
    end
    
    def self.draw_ui()
        Window.draw( UI[:side][:left][:x], UI[:side][:left][:y], UI[:side][:left][:image], UI_draw_order)
        Window.draw( UI[:side][:right][:x], UI[:side][:right][:y], UI[:side][:right][:image], UI_draw_order)
        Window.draw( UI[:under][:x], UI[:under][:y], UI[:under][:image], UI_draw_order)
        
        Window.draw_font( UI[:pause][:x], UI[:pause][:y], UI[:pause][:string], UI_Font, option={:z=>UI_draw_font_order}) if @@pause
    end
end