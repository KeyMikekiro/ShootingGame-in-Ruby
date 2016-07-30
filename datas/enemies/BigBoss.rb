module BigBossForm
    First = :first
    Second = :second
    Third = :third
end
class BigBossMissile < Bullet
    def initialize( x, y)
        type = BulletType.new(Resource.image("B_BOSS_Missile"), {:x=>5,:y=>15}, 60, {:attack=>5})
    
        super( x, y, type)
        @init_speed = { :x=>0, :y=>0}
        @init_speed[:x] = @speed[:x].abs
        @init_speed[:y] = @speed[:y].abs
    end
    
    def update()
        targeting()
        super()
    end
    
    def targeting()
    end
end
class BigBoss < Enemy
    def initialize()
        middle_boss_speed = { :x=>1, :y=>1}
        middle_boss_status = { :hp=>150, :attack=>10, :point=>200}
        middle_boss_image = Resource.image("BIG_BOSS")
        
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
        #shoot_missile()
    end
    
    def shoot_missile()
        BulletManager.push_bullet( BulletFlag::Enemy, setup_bullet) if @reload_time <= 0
    end
    
    def setup_bullet()
        @reload_time = BigBossMissile.reload_time
        return BigBossMissile.new( (@sprite.x + @sprite.image.width / 2), @sprite.y)
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