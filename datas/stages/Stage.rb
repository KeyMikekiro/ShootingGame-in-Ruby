class Stage < BaseStage
    @middle_boss_flag = false
    def event
        #ここにボスの出現処理とか書いてみる。
        if Score.get() >= 200 && !@middle_boss_flag then
            puts "event enemy is attacking!!!"
            @event_enemies.push( setup_middle_boss)
        end
    end
    
    def setup_middle_boss()
        
        @middle_boss_flag = true
        return MiddleBoss.new()
    end
end
