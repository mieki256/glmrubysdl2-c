#!ruby

require "testsprite"

class Scene
  def initialize
    @img0 = ImageSurface.load("sample.png")
    p @img0.width
    p @img0.height
    p @img0.filename
    
    @img1 = ImageSurface.new(64, 64, [128, 255, 128, 64])
    p @img1.width
    p @img1.height
    p @img1.filename
    
    @sprs = []
    num = 128
    num.times do |i|
      rad = i * 360.0 / num * Math::PI / 180.0
      r = rand * 4.0 + 1.0
      dx = r * Math.cos(rad)
      dy = r * Math.sin(rad)
      img = (i <= num /2)? @img0 : @img1
      @sprs.push(TestSprite.new(320, 240, img, dx, dy))
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
