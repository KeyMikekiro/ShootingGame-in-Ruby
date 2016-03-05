class Player < UnitObject
    def initialize( x, y, image, speed, status, bullet_type)
        super( x, y, image, speed)
        @status = status
        @bullet_type = bullet_type
        @shoot_bullets = []
        @reload_time = 0
        @invincible_time = 0
        @guns = Array.new(status[:number_guns])
        
        set_gun_x = @sprite.image.width / (status[:number_guns]+1)
        for count in 0...@guns.size do
            @guns[count] = {:x=>set_gun_x * (count + 1), :reload_time=>0}
        end
    end
    attr_reader :status, :shoot_bullets
    
    def update()
        for gun in @guns do
            gun[:reload_time] -= 1 if gun[:reload_time] > 0
        end
        @invincible_time -= 1 if @invincible_time > 0
    end
    
    def colision( enemy)
        if @sprite === enemy.sprite && !dead? then
            damage(enemy)
            return self
        end
        return nil
    end
    
    def damage( unit)
        if unit.kind_of?( UnitObject) && @invincible_time <= 0 then
            @status[:hp] -= unit.status[:attack]
            @invincible_time = @status[:invincible_time]
        end
    end
    
    def draw()
        super if @invincible_time % 2 == 0 && !dead?
        
        font = Fonts::Middle
        for count in 0...@guns.size do
            GameWindow.debug_draw_font( 0, 200 + font.size * count, 
                "Gun: " + @guns[count][:reload_time].to_s, font)
        end
        
        GameWindow.draw_font( 0, 0, "HP: " + @status[:hp].to_s, Fonts::Large)
    end
    
    def input()
        input_UD = false
        input_LR = false
        input_UD = Input.key_down?(K_W) || Input.key_down?(K_S)
        input_LR = Input.key_down?(K_A) || Input.key_down?(K_D)
        
        if input_UD && input_LR then
            if Input.key_down?(K_W) then
                @sprite.y -= @speed[:dia_y] if @sprite.y >= GameWindow.y
            elsif Input.key_down?(K_S) then
                @sprite.y += @speed[:dia_y] if @sprite.y <= GameWindow.height - @sprite.image.height
            end
            
            if Input.key_down?(K_A) then
                @sprite.x -= @speed[:dia_x] if @sprite.x >= GameWindow.x
            elsif Input.key_down?(K_D) then
                @sprite.x += @speed[:dia_x] if @sprite.x <= GameWindow.width - @sprite.image.width
            end
        else
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
        end
        
        #shoot bullet process.
        #弾発射♂処理
        if Input.key_down?(K_SPACE) then
            for gun in @guns do
                if gun[:reload_time] <= 0 then
                    BulletManager.push_bullet( BulletFlag::Player, setup_bullet(gun))
                    break
                end
            end
        end
    end
    
    def setup_bullet(gun)
        gun[:reload_time] = @bullet_type.reload_time
        return Bullet.new( (@sprite.x + gun[:x]), @sprite.y, @bullet_type.dup)
    end
    
    def debug()
        GameWindow.debug_draw_font( 0, 0, "bullet_length: " + @shoot_bullets.length.to_s, Fonts::Middle)
    end
end