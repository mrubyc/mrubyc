# MegaMrbc.draw_text("Hello, Megadrive from ruby/c")

def draw_text(str, x, y)
  MegaMrbc.draw_text(str, x, y)
end

def joypad_state(pad_num)
  MegaMrbc.read_joypad(pad_num)
end

def wait_vblank
  MegaMrbc.wait_vblank
end

class Game
  def main
    while true do
      draw_text("press start", 0, 10)
      wait_start
      draw_text("           ", 0, 10)
      GameRound.new.game_loop
    end
  end

  def wait_start
    state = 0
    prev = 0
    while true do
      state = joypad_state(0)
      break if (state & 0x80 & ~prev) != 0
      wait_vblank
    end
  end
end

class GameRound
  KEYBOARD = [
    ["Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P"],
    ["A", "S", "D", "F", "G", "H", "J", "K", "L", "<"],
    ["Z", "X", "C", "V", "B", "N", "M", " ", "", ""]
  ]

  def game_loop
    @answer = "MRUBY"
    @curr_x = 0
    @curr_y = 0
    # current buffer is always 5 in length
    @curr_buf = "     "
    @curr_index = 0
    draw_grid(12, 0)
    draw_kb(5, 19)
    draw_tick
    
    prev_state = 0
    pad_state = 0
    running = true
    while running do
      # draw_text("#{@curr_x}, #{@curr_y}", 0, 0)
      # draw_text("#{@curr_buf}", 0, 2)
      MegaMrbc.show_cursor(@curr_x * 3 + 5, @curr_y * 3 + 19)
      pad_state = joypad_state(0)
      render_guess
      # draw_text("#{pad_state}", 0, 1)
      move_cursor(pad_state, prev_state)
      won = accept_letter(pad_state, prev_state)
      if won
        # finish game
        draw_text("YOU WIN!", 0, 4)
        break
      elsif @curr_index > 2 # 5
        draw_text("GAME OVER!", 0, 3)
        break
      # game over
      end
      prev_state = pad_state
      wait_vblank
    end
  end

  def accept_letter(state, prev)
    if (state & 0x40 & ~prev) != 0
      c = current_char(@curr_x, @curr_y)
      len = @curr_buf.strip.length
      if c == "<"
        @curr_buf[len-1] = " " if len > 0
      elsif c == " "
        if len == 5
          colourise_guess # unimplemented
          return true if @curr_buf == @answer
          reset_buf
          @curr_index += 1
        end
      elsif len < 5
        #draw_text("adding letter #{c}", 0, 6)
        #draw_text("state, prev #{state}, #{prev}    ", 0, 7)
        @curr_buf[len] = c
      end
    end
    return false
  end

  def current_char(x, y)
    KEYBOARD[y] && KEYBOARD[y][x]
  end

  def render_guess
    [0, 1, 2, 3, 4].each do |i|
      draw_text(@curr_buf[i], 13 + 3 * i, @curr_index * 3 + 1)
    end
  end

  def colourise_guess
    [0, 1, 2, 3, 4].each do |i|
      if @curr_buf[i] == @answer[i]
        # green
        MegaMrbc.draw_green_square(13 + 3 * i, @curr_index * 3 + 1)
      elsif @answer.include? @curr_buf[i]
        # yellow
        MegaMrbc.draw_yellow_square(13 + 3 * i, @curr_index * 3 + 1)
      end
    end
  end

  def reset_buf
    [0, 1, 2, 3, 4].each do |i| 
      @curr_buf[i] = " "
    end
  end

  def move_cursor(state, prev)
    case
    when (state & 0x01 & ~prev) != 0
      move_up
    when (state & 0x04 & ~prev) != 0
      move_left
    when (state & 0x02 & ~prev) != 0
      move_down
    when (state & 0x08 & ~prev) != 0
      move_right
    else
      # nothing
    end
  end

  def valid_position?(x, y)
    KEYBOARD[y] && KEYBOARD[y][x] && KEYBOARD[y][x] != ""
  end

  def move_up
    new_y = @curr_y - 1
    if new_y >= 0 && valid_position?(@curr_x, new_y)
      @curr_y = new_y
    end
  end

  def move_left
    new_x = @curr_x - 1
    if new_x >= 0 && valid_position?(new_x, @curr_y)
      @curr_x = new_x
    end
  end

  def move_down
    new_y = @curr_y + 1
    if new_y >= 0 && valid_position?(@curr_x, new_y)
      @curr_y = new_y
    end
  end

  def move_right
    new_x = @curr_x + 1
    if new_x >= 0 && valid_position?(new_x, @curr_y)
      @curr_x = new_x
    end
  end

  def draw_tick
    MegaMrbc.show_tick(5 + 22, 19 + 7)
  end

  def draw_grid(x, y)
    [0, 1, 2, 3, 4, 5].each do |i|
      [0, 1, 2, 3, 4].each do |j|
        draw_rect(x + j * 3, y + i * 3)
      end
    end
  end

  # draws a 3x3 rectangle with x, y as top-left
  def draw_rect(x, y)
    MegaMrbc.draw_top_left(x, y)
    MegaMrbc.draw_top_centre(x + 1, y)
    MegaMrbc.draw_top_right(x + 2, y)
    MegaMrbc.draw_left(x, y + 1)
    MegaMrbc.draw_right(x + 2, y + 1)
    MegaMrbc.draw_bottom_left(x, y + 2)
    MegaMrbc.draw_bottom_centre(x + 1, y + 2)
    MegaMrbc.draw_bottom_right(x + 2, y + 2)
  end

  def draw_kb(x, y)
    [0, 1, 2].each do |i|
      [0, 1, 2, 3, 4, 5, 6, 7, 8, 9].each do |j|
        draw_rect(x + j * 3, y + i * 3) unless i == 2 && j > 7
        MegaMrbc.draw_text(KEYBOARD[i][j], x + j * 3 + 1, y + i * 3 + 1)
      end
    end
  end
end

Game.main
