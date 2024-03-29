import * as React from "react"

import Autoplay from "embla-carousel-autoplay"
import {
  Carousel,
  CarouselContent,
  CarouselItem,
  CarouselNext,
  CarouselPrevious,
} from "./ui/carousel"

const galleryContent = [
    {
        type: 'image',
        source: 'https://user-images.githubusercontent.com/3129530/194735416-3f386437-456c-4145-9b5e-6bb6451d7e9a.png'
    },
    {
        type: 'image',
        source: 'https://user-images.githubusercontent.com/3129530/194735376-3a476643-00d6-4cfc-9a88-e0dd33658564.png'
    },
    {
        type: 'video',
        source: 'https://user-images.githubusercontent.com/3129530/194734947-f34bc377-8ee4-472a-b130-30ce9e86facf.webm'
    }
]

export function GalleryCarousel() {
  return (
    <Carousel
        className="w-full max-w-4xl"
        plugins={[
            Autoplay({
                delay: 2000
            }),
        ]}
    >
      <CarouselContent>
        {galleryContent.map((data, index) => (
          <CarouselItem key={index}>
            <div className="rounded-md overflow-hidden aspect-video">
                {data.type === 'image' &&
                    <img src={data.source} />
                }
                {data.type === 'video' &&
                    <video className="object-contain" src="https://user-images.githubusercontent.com/3129530/194735274-395839e9-e2cb-4a47-83b9-5c6987640032.webm" autoPlay />
                }
            </div>
          </CarouselItem>
        ))}
      </CarouselContent>
      <CarouselPrevious />
      <CarouselNext />
    </Carousel>
  )
}
