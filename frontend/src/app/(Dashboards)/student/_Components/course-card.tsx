"use client"

import { Button } from "@/components/ui/button"
import { Card, CardContent } from "@/components/ui/card"
import { Badge } from "@/components/ui/badge"
import { Check, Plus, Star } from "lucide-react"

interface CourseCardProps {
  courseCode: string
  credits: number
  title: string
  description: string
  isEnrolled: boolean
  onToggleEnroll: () => void
}

export function CourseCard({
  courseCode,
  credits,
  title,
  description,
  isEnrolled,
  onToggleEnroll,
}: CourseCardProps) {
  return (
    <>
      {/* Mobile Card with colored left border */}
      <Card className="overflow-hidden border-l-4 border-l-primary md:hidden">
        <CardContent className="p-4">
          <div className="mb-3 flex items-center justify-between">
            <Badge variant="outline" className="rounded-md bg-muted/50 font-normal text-foreground">
              {courseCode}
            </Badge>
            <div className="flex items-center gap-1.5 text-sm text-muted-foreground">
              <Star className="h-4 w-4" />
              <span>{credits} Credits</span>
            </div>
          </div>
          <h3 className="mb-2 text-base font-semibold text-foreground">{title}</h3>
          <p className="mb-4 line-clamp-2 text-sm text-muted-foreground">{description}</p>
          <Button
            onClick={onToggleEnroll}
            variant={isEnrolled ? "outline" : "default"}
            className={`w-full gap-2 ${isEnrolled ? "border-muted bg-muted/50 text-foreground hover:bg-muted" : "bg-primary hover:bg-primary/90"}`}
          >
            {isEnrolled ? (
              <>
                <Check className="h-4 w-4" />
                Unenroll
              </>
            ) : (
              <>
                <Plus className="h-4 w-4" />
                Enroll
              </>
            )}
          </Button>
        </CardContent>
      </Card>

      {/* Desktop Card matching the original design */}
      <Card className="hidden overflow-hidden md:block">
        <CardContent className="p-6">
          <div className="mb-4 flex items-center justify-between">
            <Badge variant="outline" className="rounded-md bg-muted/50 font-normal text-foreground">
              {courseCode}
            </Badge>
            <Badge className="rounded-full bg-accent text-accent-foreground">
              {credits} Credits
            </Badge>
          </div>
          <h3 className="mb-2 text-lg font-semibold text-foreground">{title}</h3>
          <p className="mb-6 line-clamp-2 text-sm text-muted-foreground">{description}</p>
          <Button
            onClick={onToggleEnroll}
            variant={isEnrolled ? "outline" : "default"}
            className={`w-full ${isEnrolled ? "border-destructive/30 text-destructive hover:bg-destructive/5" : "bg-primary hover:bg-primary/90"}`}
          >
            {isEnrolled ? "Unenroll" : "Enroll"}
          </Button>
        </CardContent>
      </Card>
    </>
  )
}
