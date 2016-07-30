module BossSince

end

class TestStage < BaseStage
    def initialize( game_admin, encount_enemy_num=0, encount_time=0, mapdata=nil)
        super( game_admin, encount_enemy_num, encount_time, mapdata)
        @boss_flag = :none
    end
    
    def event
        #ここにボスの出現処理とか書いてみる。
        #ここクソコードじゃね？
        # if Score.get() >= 200 && @boss_flag == :none then
        #     @event_enemies.push( setup_middle_boss())
        #     @boss_flag = :middle
        # end
        
        if Score.get() >= 500 && @boss_flag == :none then
            @event_enemies.push( setup_big_boss())
            @boss_flag = :big
        end
    end
    
    #イベント用の敵が出現するときにモブも表示させるかはこっちで判断させる。
    def update
        enable_encount() if !has_event_enemy()
        super()
    end

    def setup_middle_boss()
        disable_encount()
        deleteAllEnemy()
        return MiddleBoss.new()
    end
    
    def setup_big_boss()
        disable_encount()
        deleteAllEnemy()
        return BigBoss.new()
    end
end
