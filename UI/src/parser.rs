use image::GenericImageView;

#[derive(Debug, Clone, Copy, PartialEq)]
pub struct Rgb(pub u8, pub u8, pub u8);

pub struct TerminalAsset {
    pub width: u32,
    pub height: u32,
    pub cells: Vec<(Option<usize>, Option<usize>)>,
}

pub const PALETTE: [Rgb; 7] = [
    Rgb(83, 88, 113),
    Rgb(0, 0, 0),
    Rgb(94, 104, 185),
    Rgb(138, 151, 227),
    Rgb(191, 200, 255),
    Rgb(133, 151, 255),
    Rgb(255, 255, 255),
];

pub fn parse_png_to_asset(path: &str) -> TerminalAsset {
    let img = image::open(path).expect("Failed to open image");
    let (width, img_height) = img.dimensions();
    let mut cells = Vec::new();

    let find_idx = |pixel: image::Rgba<u8>| -> Option<usize> {
        if pixel[3] == 0 {
            return None;
        }
        let (r, g, b) = (pixel[0], pixel[1], pixel[2]);
        PALETTE
            .iter()
            .position(|&p| p.0 == r && p.1 == g && p.2 == b)
            .or(Some(0))
    };

    for y in (0..img_height).step_by(2) {
        for x in 0..width {
            let top_idx = find_idx(img.get_pixel(x, y));
            let bot_idx = if y + 1 < img_height {
                find_idx(img.get_pixel(x, y + 1))
            } else {
                None
            };
            cells.push((top_idx, bot_idx));
        }
    }

    TerminalAsset {
        width,
        height: img_height.div_ceil(2),
        cells,
    }
}

impl ratatui::widgets::Widget for &TerminalAsset {
    fn render(self, area: ratatui::layout::Rect, buf: &mut ratatui::buffer::Buffer) {
        let render_width = std::cmp::min(self.width as u16, area.width);
        let render_height = std::cmp::min(self.height as u16, area.height);

        for y in 0..render_height {
            for x in 0..render_width {
                let idx = (y as usize * self.width as usize) + x as usize;
                let (top_idx, bot_idx) = self.cells[idx];

                let get_color = |idx: Option<usize>| -> ratatui::style::Color {
                    match idx {
                        Some(i) => {
                            ratatui::style::Color::Rgb(PALETTE[i].0, PALETTE[i].1, PALETTE[i].2)
                        }
                        None => ratatui::style::Color::Reset,
                    }
                };

                // Use cell_mut with a tuple (x, y) to satisfy the new Position requirement
                if let Some(cell) = buf.cell_mut((area.x + x, area.y + y)) {
                    cell.set_char('▀')
                        .set_fg(get_color(top_idx))
                        .set_bg(get_color(bot_idx));
                }
            }
        }
    }
}
