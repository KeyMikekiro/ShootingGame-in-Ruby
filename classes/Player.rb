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
    attr_reader :status, :shoot_bullets, :x, :y
    
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
        super if @invincible_time <= 0
        draw_alpha( 125) if @invincible_time > 0
        
        font = Fonts::Middle
        for count in 0...@guns.size do
            GameWindow.debug_draw_font( 0, 200 + font.size * count, 
                "Gun: " + @guns[count][:reload_time].to_s, font)
        end
        
        GameWindow.draw_font( 0, 0, "HP: " + @status[:hp].to_s, Fonts::Large)
    end
    
    #入力処理
    def input()
        input_UD = Input.key_down?(K_W) || Input.key_down?(K_S)
        input_LR = Input.key_down?(K_A) || Input.key_down?(K_D)

        input_UD = Input.key_down?(K_I) || Input.key_down?(K_K)
        input_LR = Input.key_down?(K_J) || Input.key_down?(K_L)
        
        #斜め処理
        if input_UD && input_LR then
            #右利き用の入力処理
            if Input.key_down?(K_W) then #左に動く
                @sprite.y -= @speed[:dia_y] if @sprite.y >= GameWindow.y
            elsif Input.key_down?(K_S) then #右に動く
                @sprite.y += @speed[:dia_y] if @sprite.y <= GameWindow.height - @sprite.image.height
            end
            
            if Input.key_down?(K_A) then #上に動く
                @sprite.x -= @speed[:dia_x] if @sprite.x >= GameWindow.x
            elsif Input.key_down?(K_D) then #下に動く
                @sprite.x += @speed[:dia_x] if @sprite.x <= GameWindow.width - @sprite.image.width
            end
            #ここまで
            
            #左利き用の入力処理
            if Input.key_down?(K_I) then #左に動く
                @sprite.y -= @speed[:dia_y] if @sprite.y >= GameWindow.y
            elsif Input.key_down?(K_K) then #右に動く
                @sprite.y += @speed[:dia_y] if @sprite.y <= GameWindow.height - @sprite.image.height
            end
            if Input.key_down?(K_J) then #上に動く
                @sprite.x -= @speed[:dia_x] if @sprite.x >= GameWindow.x
            elsif Input.key_down?(K_L) then #下に動く
                @sprite.x += @speed[:dia_x] if @sprite.x <= GameWindow.width - @sprite.image.width
            end
            #ここまで
        else
            #右利き用の入力処理
            if Input.key_down?(K_W) then #左に動く
                @sprite.y -= @speed[:y] if @sprite.y >= GameWindow.y
            elsif Input.key_down?(K_S) then #右に動く
                @sprite.y += @speed[:y] if @sprite.y <= GameWindow.height - @sprite.image.height
            end
            
            if Input.key_down?(K_A) then #上に動く
                @sprite.x -= @speed[:x] if @sprite.x >= GameWindow.x
            elsif Input.key_down?(K_D) then #下に動く
                @sprite.x += @speed[:x] if @sprite.x <= GameWindow.width - @sprite.image.width
            end
            #ここまで
            
            #左利き用の入力処理
            if Input.key_down?(K_I) then #左に動く
                @sprite.y -= @speed[:y] if @sprite.y >= GameWindow.y
            elsif Input.key_down?(K_K) then #右に動く
                @sprite.y += @speed[:y] if @sprite.y <= GameWindow.height - @sprite.image.height
            end
            if Input.key_down?(K_J) then #上に動く
                @sprite.x -= @speed[:x] if @sprite.x >= GameWindow.x
            elsif Input.key_down?(K_L) then #下に動く
                @sprite.x += @speed[:x] if @sprite.x <= GameWindow.width - @sprite.image.width
            end
            #ここまで
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