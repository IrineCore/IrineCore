USE [dnworld]
GO
/****** Object:  Table [dbo].[PresetSkills]    Script Date: 11/14/2026 8:54:16 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[PresetSkills](
	[CharacterID] [bigint] NOT NULL,
	[SkillPresetIndex] [tinyint] NOT NULL,
	[SkillID] [int] NOT NULL,
	[SkillLevel] [tinyint] NOT NULL,
 CONSTRAINT [PK_PresetSkills] PRIMARY KEY CLUSTERED 
(
	[CharacterID] ASC,
	[SkillPresetIndex] ASC,
	[SkillID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
ALTER TABLE [dbo].[PresetSkills]  WITH CHECK ADD  CONSTRAINT [FK_SkillPresets_PresetSkills_CharacterID_SkillPresetIndex] FOREIGN KEY([CharacterID], [SkillPresetIndex])
REFERENCES [dbo].[SkillPresets] ([CharacterID], [SkillPresetIndex])
GO
ALTER TABLE [dbo].[PresetSkills] CHECK CONSTRAINT [FK_SkillPresets_PresetSkills_CharacterID_SkillPresetIndex]
GO
