USE [dnworld]
GO
/****** Object:  Table [dbo].[SecondarySkills]    Script Date: 11/14/2026 8:54:16 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[SecondarySkills](
	[CharacterID] [bigint] NOT NULL,
	[SecondarySkillID] [int] NOT NULL,
	[SecondarySkillTypeCode] [tinyint] NOT NULL,
	[SecondarySkillExp] [int] NOT NULL,
 CONSTRAINT [PK_SecondarySkills] PRIMARY KEY CLUSTERED 
(
	[CharacterID] ASC,
	[SecondarySkillID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
ALTER TABLE [dbo].[SecondarySkills]  WITH CHECK ADD  CONSTRAINT [CK_SecondarySkills_SecondarySkillExp] CHECK  (([SecondarySkillExp]>=(0)))
GO
ALTER TABLE [dbo].[SecondarySkills] CHECK CONSTRAINT [CK_SecondarySkills_SecondarySkillExp]
GO
