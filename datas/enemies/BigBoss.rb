module BigBossForm
    First = :first
    Second = :second
    Third = :third
end
class BigBossMissile < Bullet
    ReloadTime = 120
    def initialize( player, x, y)
        type = BulletType.new(Resource.image("B_BOSS_Missile"), {:x=>5,:y=>1}, 60, {:attack=>5})
    
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
        x = ((@player.sprite.x-@sprite.x)/(@player.sprite.x-@sprite.x).abs)*@sprite.x+@sprite.image.width*0.5
        y = @sprite.y+@sprite.image.height
        @angle = Math.asinh((@player.sprite.y-y)/((@player.sprite.x-x)**2+(@player.sprite.y-y)**2)**(0.5))

        @speed[:x] = (x/x.abs)*@init_speed[:x]
    end
end
class BigBoss < Enemy
    def initialize( game_admin)
        middle_boss_speed = { :x=>1, :y=>1}
        middle_boss_status = { :hp=>150, :attack=>10, :point=>200}
        middle_boss_image = Resource.image("BIG_BOSS")
        
        @game_admin = game_admin

        @form = BigBossForm::First
        
        #bullet = Bullet.new()
        
        super( GameWindow.center_horizontal - middle_boss_image.width / 2, 
            GameWindow.y, middle_boss_image, middle_boss_speed, middle_boss_status)
        
        @init_speed = { :x=>0, :y=>0}
        @init_speed[:x] = @speed[:x].abs
        @init_speed[:y] = @speed[:y].abs
    end

    def moving_pattern()
        if Window.height / 2 < @sprite.y then
            @speed[:y] = 0
            @speed[:x] = -5
        end
        if @speed[:y] <= 0 then
            @speed[:x] = -@init_speed[:x].abs
        end
        
        if GameWindow.x > @sprite.x && @speed[:x] < 0 then
            @speed[:x] = 0
        end
        if GameWindow.width < @sprite.x + @sprite.image.width && @speed[:x] > 0 then
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