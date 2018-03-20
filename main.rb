#!ruby

require "testsprite"

class Scene
  def initialize
    # @img = ImageSurface.new(32, 32, [128, 255, 128, 64])
    @img = ImageSurface.load("sample.png")
    p @img.width
    p @img.height
    p @img.filename
    
    @sprs = []
    num = 128
    num.times do |i|
      rad = i * 360.0 / num * Math::PI / 180.0
      r = rand * 4.0 + 1.0
      dx = r * Math.cos(rad)
      dy = r * Math.sin(rad)
      @sprs.push(TestSprite.new(320, 240, @img, dx, dy))
    end
  end

  def update
    @sprs.each { |s| s.update }
  end

  def draw
    @sprs.each { |s| s.draw }
  end
end

Scene.new
