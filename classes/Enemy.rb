class Enemy < UnitObject
    def initialize( x, y, image, speed, status, bullet=nil)
        super( x, y, image, speed)
        @status = status.dup
        @shoot_bullets = []
        @bullet_type = bullet
        @reload_time = 0
    end
    attr_reader :status
    
    def update
        super()
        @reload_time -= 1 if @reload_time > 0
        for shoot_bullet in @shoot_bullets
            shoot_bullet.update
        end
        shoot() if @bullet_type != nil
    end
    
    def shoot()
        BulletManager.push_bullet( BulletFlag::Enemy, setup_bullet) if @reload_time <= 0
    end 
    
    def setup_bullet()
        @reload_time = @bullet_type.reload_time
        return Bullet.new( (@sprite.x + @sprite.image.width / 2), @sprite.y, @bullet_type.dup)
    end
    
    def damage( unit)
        if unit.kind_of?(UnitObject) then
            UnitSound::Explosive.play( 1, 0).set_volume(80)
            @status[:hp] -= unit.status[:attack]
        end
    end
    
    def draw
        super
        
        GameWindow.debug_draw_font( @sprite.x, @sprite.y, @status[:hp].to_s, Fonts::Middle)
    end
end