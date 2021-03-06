module MiddleBossForm
    First = :first
    Second = :second
    Third = :third
end
class MiddleBoss < Enemy
    def initialize()
        middle_boss_speed = { :x=>0, :y=>1}
        middle_boss_status = { :hp=>150, :attack=>10, :point=>200}
        middle_boss_image = Resource.image("Middle_BOSS")
        
        @form = MiddleBossForm::First
        
        super( GameWindow.center_horizontal - middle_boss_image.width / 2, 
            GameWindow.y, middle_boss_image, middle_boss_speed, middle_boss_status)
            
        @init_speed = @speed
    end

    def moving_pattern()
        if Window.height / 2 < @sprite.y then
            @speed[:y] = 0
            @speed[:x] = -5
        end
        if @sprite.x < GameWindow.x then
            @speed[:y] = -5
            @speed[:x] = 3
        end
        if @sprite.y <= 0 then
            @speed[:y] = 5
            @speed[:x] = 0
        end
        slope_cluc()
    end
    
    
    def update()
        moving_pattern()
        super()
        action()
    end
    
    
    def draw()
        super()
        GameWindow.draw_font( position_width, @sprite.y, "Test Test.", Fonts::Middle) if @status[:hp] < 50
    end
    
    
    def action()
        if @status[:hp] <= 0 then
            case @form
            when MiddleBossForm::First then
                second_form()
            when MiddleBossForm::Second then
                third_form()
            end
        end
    end
    
    def second_form
        @status[:hp] = 400
        @sprite.image = Resource.image("Middle_BOSS_2")
        @form = MiddleBossForm::Second
    end
    
    def third_form
        @status[:hp] = 600
        @sprite.image = Resource.image("Middle_BOSS_3")
        @form = MiddleBossForm::Third
    end
end
