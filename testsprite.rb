#!ruby

class TestSprite
  
  attr_accessor :x, :y, :image
  
  def initialize(x, y, image, dx, dy)
    @x = x
    @y = y
    @image = image
    @dx = dx
    @dy = dy
    @w = image.width
    @h = image.height
  end

  def update
    wh = @w / 2
    hh = @h / 2
    @x += @dx
    @y += @dy
    @dx *= -1 if (@x <= wh or @x >= 640 - wh)
    @dy *= -1 if (@y <= hh or @y >= 480 - hh)
  end
  
  def draw
    x = @x - (@w / 2)
    y = @y - (@h / 2)
    @image.draw_to_window(x, y)
  end
end
