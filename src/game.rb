# MegaMrbc.draw_text("Hello, Megadrive from ruby/c")

class Game
  def main
    while true do
      GameRound.game_loop
    end
  end
end

class GameRound
  def self.game_loop
    running = true
    while running do
      [0,1].each do |i|
        pad_state = joypad_state(i)
        draw_text("Joypad state: #{pad_state}    ", 4, 20+i)

        wait_vblank
      end
    end
  end

  def self.joypad_state(pad_num)
    MegaMrbc.read_joypad(pad_num)
  end

  def self.draw_text(str, x, y)
    MegaMrbc.draw_text(str, x, y)
  end

  def self.wait_vblank
    MegaMrbc.wait_vblank
  end
end

Game.main
