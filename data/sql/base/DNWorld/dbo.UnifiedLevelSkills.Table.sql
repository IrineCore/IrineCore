USE [dnworld]
GO
/****** Object:  Table [dbo].[UnifiedLevelSkills]    Script Date: 11/14/2026 8:54:16 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[UnifiedLevelSkills](
	[CharacterID] [bigint] NOT NULL,
	[UnifiedLevelSkillIndex] [tinyint] NOT NULL,
	[UnifiedLevelSkillID] [int] NOT NULL,
 CONSTRAINT [PK_UnifiedLevelSkills] PRIMARY KEY CLUSTERED 
(
	[CharacterID] ASC,
	[UnifiedLevelSkillIndex] ASC,
	[UnifiedLevelSkillID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
