class Stage < BaseStage
    
    def event
        #ここにボスの出現処理とか書いてみる。
        if Score.get() >= 400 then
            #@event_enemies.push( { "middle_boss"=>""})
        end
    end
end
