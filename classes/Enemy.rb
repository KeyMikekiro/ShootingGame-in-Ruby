class Enemy < UnitObject
    def initialize( x, y, image, speed, status)
        super( x, y, image, speed)
        @status = status
    end
    attr_reader :status
    
    def damage( unit)
        if unit != nil then
            UnitSound::Explosive.play( 1, 0).set_volume(80)
            @status[:hp] -= unit.status[:attack] 
        end
    end
    
    def draw
        super
        GameWindow.debug_draw_font( @sprite.x, @sprite.y, @status[:hp].to_s, Fonts::Middle)
    end
    
    def dead?
        return @status[:hp] <= 0 || vanished?
    end
end