class TestStage < BaseStage
    @middle_boss = nil
    @big_boss = nil
    def event
        #ここにボスの出現処理とか書いてみる。
        if Score.get() >= 200 && @middle_boss == nil then
            puts "event enemy is attacking!!!"
            @middle_boss = setup_middle_boss()
            @event_enemies.push( @middle_boss)
        end
        
        if Score.get() >= 1200 && @big_boss == nil then
            @big_boss = setup_big_boss()
            @event_enemies.push( @big_boss)
        end
    end
    
    def update
        start_encount?( @middle_boss) if @middle_boss != nil
        start_encount?( @big_boss) if @big_boss != nil
        super()
    end

    def setup_middle_boss()
        @stop_encount = true
        return MiddleBoss.new()
    end
    
    def setup_big_boss()
        @stop_encount = true
        return BigBoss.new()
    end
end
