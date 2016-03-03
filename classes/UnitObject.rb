class UnitObject
    def initialize( x, y, image, speed)
        @sprite = Sprite.new( x, y, image)
        @speed = speed
    end
    attr_reader :sprite
    
    def update
        @sprite.x += @speed[:x]
        @sprite.y += @speed[:y]
        
        @sprite.vanish if ( @sprite.y < -@sprite.image.height || @sprite.y > Window.height + @sprite.image.height) ||
                         ( @sprite.x < -@sprite.image.width || @sprite.x > Window.width + @sprite.image.width)
        
    end
    
    def draw
        @sprite.draw
    end
    
    def vanished?
        return @sprite.vanished?
    end
end
