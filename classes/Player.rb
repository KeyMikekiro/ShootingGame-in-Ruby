class Player < UnitObject
    def initialize( x, y, image, speed, status, bullet_type)
        super( x, y, image, speed)
        @status = status
        @bullet_type = bullet_type
        @shoot_bullets = []
        @reload_time = 0
    end
    attr_reader :status, :shoot_bullets
    
    def update()
        @reload_time -= 1 if @reload_time > 0
        for shoot_bullet in @shoot_bullets
            shoot_bullet.update
            #@shoot_bullets.delete( shoot_bullet) if shoot_bullet.vanished?
        end
        Sprite.clean( @shoot_bullets)
    end
    
    def colision( enemy)
        return self if @sprite === enemy.sprite
        for bullet in @shoot_bullets do
            if bullet.sprite === enemy.sprite then
                @shoot_bullets.delete( bullet)
                return bullet
            end
        end
        return nil
    end
    
    def draw()
        super
        Sprite.draw( @shoot_bullets)
        
        GameWindow.draw_font( 0, 0, "HP: " + @status[:hp].to_s, Fonts::Large)
    end
    
    def input()
        if Input.key_down?(K_W) then
            @sprite.y -= @speed[:y] if @sprite.y >= GameWindow.y
        elsif Input.key_down?(K_S) then
            @sprite.y += @speed[:y] if @sprite.y <= GameWindow.height - @sprite.image.height
        end
        if Input.key_down?(K_A) then
            @sprite.x -= @speed[:x] if @sprite.x >= GameWindow.x
        elsif Input.key_down?(K_D) then
            @sprite.x += @speed[:x] if @sprite.x <= GameWindow.width - @sprite.image.width
        end
        
        if Input.key_down?(K_SPACE) && @reload_time == 0 then
            @shoot_bullets.push( setup_bullet)
        end
    end
    
    def setup_bullet()
        @reload_time = @bullet_type.reload_time
        return Bullet.new( (@sprite.x + @sprite.image.width / 2), @sprite.y, @bullet_type.dup)
    end
    
    def debug()
        GameWindow.debug_draw_font( 0, 0, "bullet_length: " + @shoot_bullets.length.to_s, Fonts::Middle)
    end
end