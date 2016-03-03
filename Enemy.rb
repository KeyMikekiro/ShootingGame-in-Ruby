class Enemy < UnitObject
    def initialize( x, y, image, speed, status)
        super( x, y, image, speed)
        @status = status
    end
    attr_reader :status
    
    def damage( unit)
        @status[:hp] -= unit.status[:attack] if unit != nil
    end
    
    def draw
        super
        Window.draw_font( @sprite.x, @sprite.y, @status[:hp].to_s, Fonts::Middle)
    end
    
    def dead?
        return @status[:hp] <= 0
    end
end