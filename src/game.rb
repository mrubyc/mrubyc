# MyClass.func("Hello, Megadrive from ruby/c")

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
      #MyClass.func("Hello, Megadrive from ruby/c")
      pad_state = joypad_state(0);
      draw_text("Joypad state: #{pad_state}")
      #wait_vsync
    end
  end

  def self.joypad_state(pad_num)
    123
  end

  def self.draw_text(str)
    MyClass.func(str)
  end

  def self.wait_vsync
    # to be implemented
  end
end

Game.main
