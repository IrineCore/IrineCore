USE [dnworld]
GO
/****** Object:  Table [dbo].[Skills]    Script Date: 11/14/2026 8:54:16 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[Skills](
	[CharacterID] [bigint] NOT NULL,
	[SkillID] [int] NOT NULL,
	[SkillLevel] [tinyint] NOT NULL,
	[CoolTime] [int] NOT NULL,
	[UsedSkillPoint] [smallint] NOT NULL,
	[DefaultSkillLevel] [tinyint] NOT NULL,
	[SkillCode] [tinyint] NOT NULL,
 CONSTRAINT [PK_Skills] PRIMARY KEY CLUSTERED 
(
	[CharacterID] ASC,
	[SkillCode] ASC,
	[SkillID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
