<template>
  <div class="juce-slider">
    <!-- <span class="value-text">
      {{ modelValue.toFixed(2) }}
    </span> -->
    <div
      v-if="knobImage"
      class="knob"
      :style="{
        backgroundImage: `url('${knobImage}')`,
        backgroundPosition: `0px -${currentFrame * frameHeight}px`,
        backgroundSize: `100% auto`,
      }"
      @mousedown="startDrag"
      @touchstart.prevent="startDrag"
    ></div>

    <input
      v-else
      type="range"
      :min="0"
      :max="1"
      :step="0.01"
      :value="modelValue"
      @input="onInput"
      class="w-48"
    />
    <span class="label-text">
      {{ label }}
    </span>
  </div>
</template>

<script setup lang="ts">
import { ref, watch } from "vue";

// props
const props = defineProps<{
  label: string;
  modelValue: number;
  knobImage?: string;
  frames?: number; // number of frames in the knob sprite
}>();

const emit = defineEmits<{
  (e: "update:modelValue", value: number): void;
}>();

const frameHeight = 75; // pixels, match knob sprite

const currentFrame = ref(0);

function updateFrame() {
  const totalFrames = props.frames || 1;
  currentFrame.value = Math.min(
    totalFrames - 1,
    Math.max(0, Math.round(props.modelValue * (totalFrames - 1)))
  );
}

watch(() => props.modelValue, updateFrame, { immediate: true });

function startDrag(e: MouseEvent | TouchEvent) {
  if (!props.knobImage) return;

  const startY = "touches" in e ? e.touches[0].clientY : e.clientY;
  const startValue = props.modelValue;

  function onMove(ev: MouseEvent | TouchEvent) {
    const moveY = "touches" in ev ? ev.touches[0].clientY : ev.clientY;

    const delta = (startY - moveY) / 150;
    let newValue = startValue + delta;
    newValue = Math.max(0, Math.min(1, newValue));
    emit("update:modelValue", newValue);
  }

  function onUp() {
    window.removeEventListener("mousemove", onMove);
    window.removeEventListener("mouseup", onUp);
    window.removeEventListener("touchmove", onMove);
    window.removeEventListener("touchend", onUp);
  }

  window.addEventListener("mousemove", onMove);
  window.addEventListener("mouseup", onUp);
  window.addEventListener("touchmove", onMove);
  window.addEventListener("touchend", onUp);
}

function onInput(e: Event) {
  const target = e.target as HTMLInputElement;
  emit("update:modelValue", parseFloat(target.value));
}
</script>

<style scoped>
.juce-slider {
  display: flex;
  flex-direction: column;
  align-items: center;
  gap: 0px;
}

/* Knob div */
.knob {
  width: 75px; /* one frame size */
  height: 75px;
  background-repeat: no-repeat;
  background-size: 100% auto;
  cursor: grab;
}

/* Value text above knob */
.value-text {
  font-size: 0.875rem;
  font-weight: 600;
  color: limegreen;
  text-align: center;
}

/* Label text under knob */
.label-text {
  font-size: 1.2rem;
  font-weight: bold;
  font-family: "Digital", monospace;
  text-align: center;
}
</style>
