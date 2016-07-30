module BigBossForm
    First = :first
    Second = :second
    Third = :third
end
class BigBossMissile < Bullet
    ReloadTime = 120
    def initialize( player, x, y)
        type = BulletType.new(Resource.image("B_BOSS_Missile"), {:x=>5,:y=>3}, 60, {:attack=>5})
    
        super( x, y, type)
        @player = player
        @init_speed = { :x=>0, :y=>0}
        @init_speed[:x] = @speed[:x].abs
        @init_speed[:y] = @speed[:y].abs

        @angle = 0
    end

    def self.reload_time
        return ReloadTime
    end

    def update()
        targeting()
        super()
    end

    def draw()
        GameWindow.draw_rot( @sprite.x, @sprite.y, @sprite.image, @angle, @sprite.image.width*0.5, @sprite.image.height*0.5)
    end
    
    def targeting()
        xsign = ((@player.sprite.x*0.5-@sprite.x*0.5)/(@player.sprite.x*0.5-@sprite.x*0.5).abs)
        ysign = ((@player.sprite.y*0.5-@sprite.y*0.5)/(@player.sprite.y*0.5-@sprite.y*0.5).abs)
        x = xsign * @sprite.x+@sprite.image.width*0.5
        y = @sprite.y+@sprite.image.height
        @angle = Math.atan2((y-@player.sprite.y*0.5),(x-@player.sprite.x))*360
        
        if((((@player.sprite.image.width*4)+(@player.sprite.image.height*4))**(0.5)) < @sprite.x) then
            @speed[:x] = xsign*@init_speed[:x]
            @speed[:y] = ysign*@init_speed[:y]
        end
    end
end
class BigBoss < Enemy
    def initialize( game_admin)
        middle_boss_speed = { :x=>1, :y=>1}
        middle_boss_status = { :hp=>150, :attack=>10, :point=>200}
        middle_boss_image = Resource.image("BIG_BOSS")
        
        @game_admin = game_admin

        @form = BigBossForm::First
        
        super( GameWindow.center_horizontal - middle_boss_image.width / 2, 
            GameWindow.y, middle_boss_image, middle_boss_speed, middle_boss_status)
        
        @init_speed = { :x=>0, :y=>0}
        @init_speed[:x] = @speed[:x].abs
        @init_speed[:y] = @speed[:y].abs
    end

    def moving_pattern()
        if Window.height / 3 < @sprite.y then
            @speed[:y] = 0
            @speed[:x] = -5
        end
        if @speed[:y] <= 0 then
            @speed[:x] = -@init_speed[:x].abs
        end
        
        if GameWindow.x > @sprite.x && @speed[:x] < 0 then
            @speed[:x] = 0
        end
        if GameWindow.width * 0.5 < @sprite.x + @sprite.image.width && @speed[:x] > 0 then
            @speed[:x] = 0
        end
        slope_cluc()
    end
    
    
    def update()
        moving_pattern()
        super()
        action()
        shoot_missile()
    end
    
    def shoot_missile()
        BulletManager.push_bullet( BulletFlag::Enemy, setup_bullet) if @reload_time <= 0
    end
    
    def setup_bullet()
        @reload_time = BigBossMissile.reload_time
        return BigBossMissile.new( @game_admin.player, (@sprite.x + @sprite.image.width / 2), @sprite.y)
    end
    
    def draw()
        super()
        GameWindow.draw_font( position_width, @sprite.y, "Test Test.", Fonts::Middle) if @status[:hp] < 50
    end
    
    
    def action()
        if @status[:hp] <= 0 then
            case @form
            when BigBossForm::First then
                second_form()
            when BigBossForm::Second then
                third_form()
            end
        end
    end
    
    def second_form
        @status[:hp] = 400
        @sprite.image = Resource.image("BIG_BOSS_2")
        @form = BigBossForm::Second
    end
    
    def third_form
        @status[:hp] = 600
        @sprite.image = Resource.image("BIG_BOSS_3")
        @form = BigBossForm::Third
    end
end