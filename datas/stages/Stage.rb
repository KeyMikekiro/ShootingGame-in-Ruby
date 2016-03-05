class Stage < BaseStage
    @middle_boss = nil
    def event
        #ここにボスの出現処理とか書いてみる。
        if Score.get() >= 200 && @middle_boss == nil then
            puts "event enemy is attacking!!!"
            @middle_boss = setup_middle_boss()
            @event_enemies.push( @middle_boss)
        end
    end
    
    def update
        #rubyの特性、object_idの一致を利用！
        @stop_encount = false if @middle_boss != nil && @middle_boss.dead?
        super()
    end
    
    def setup_middle_boss()
        @stop_encount = true
        return MiddleBoss.new()
    end
end
