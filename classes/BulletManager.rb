#BulletManager is manage to unit bullet. That is Enemy and Player bullet.
#BulletManagerは敵とプレイヤーの弾の管理をしています。
module BulletManager
    @@bullets = {}
    def self.push_bullet( flag, bullet)
        @@bullets.store(flag,[]) if @@bullets[flag] == nil
        @@bullets[flag].push(bullet)
    end
    
    def self.update()
        #一回bulletsの中身取り出さないとダメだからこうやるわ。
        for ary_bullets in @@bullets.values do
            for bullet in ary_bullets do
                bullet.update
            end
        end
        
        debug()
    end
    
    def self.draw()
        Sprite.draw( @@bullets.values)
    end
    
    def self.colision( unit, obj_class, bullet_flag)
        if unit.kind_of?(obj_class) && @@bullets[bullet_flag] != nil then
            for bullet in @@bullets[bullet_flag] do
                #puts bullet.kind_of?(Bullet)
                if unit.sprite === bullet.sprite then
                    @@bullets[bullet_flag].delete( bullet)
                    return bullet
                end
            end
        end
        return nil
    end
    
    def self.debug()
        GameWindow.debug_draw_font( 0,Window.height-Fonts::Middle.size, "bullet_count: " + @@bullets.values.size.to_s, Fonts::Middle)
    end
end
