class Stage < BaseStage
    @middle_boss_flag = false
    def event
        #ここにボスの出現処理とか書いてみる。
        if Score.get() >= 200 && !@middle_boss_flag then
            @event_enemies.push( setup_middle_boss)
        end
    end
    
    def setup_middle_boss()
        middle_boss_speed = { :x=>0, :y=>1}
        middle_boss_status = { :hp=>150, :attack=>10, :point=>200}
        middle_boss_image = Resource.image("Middle_BOSS")
        
        @middle_boss_flag = true
        
        return Enemy.new( GameWindow.center_horizontal - middle_boss_image.width / 2, 
            GameWindow.y, middle_boss_image, middle_boss_speed, middle_boss_status)
    end
end
