class UnitObject
    def initialize( x, y, image, speed)
        @sprite = Sprite.new( x, y, image)
        @speed = speed
        
        a = (@speed[:x]**2 + @speed[:y]**2)**(1/2.0)
        @speed.store(:dia_x, @speed[:x] * @speed[:x]/a)
        @speed.store(:dia_y, @speed[:y] * @speed[:y]/a)
    end
    attr_reader :sprite
    
    def update
        @sprite.x += @speed[:x]
        @sprite.y += @speed[:y]
        
        check_vanish()
    end
    
    def position_width
        return @sprite.x + @sprite.image.width
    end
    
    def position_height
        return @sprite.y + @sprite.image.height
    end
    
    def check_vanish
        @sprite.vanish if ( @sprite.y < GameWindow.y - @sprite.image.height || @sprite.y > GameWindow.height + @sprite.image.height) ||
            ( @sprite.x < GameWindow.x - @sprite.image.width / 2 || @sprite.x > GameWindow.width + @sprite.image.width / 2)
        
    end
    
    def draw
        GameWindow.draw(@sprite.x, @sprite.y, @sprite.image)
    end
    
    def vanished?
        return @sprite.vanished?
    end
    
    def dead?
        def dead?
            return @status[:hp] <= 0 || vanished?
        end
    end
end
