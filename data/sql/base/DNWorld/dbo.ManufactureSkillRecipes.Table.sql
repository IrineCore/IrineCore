USE [dnworld]
GO
/****** Object:  Table [dbo].[ManufactureSkillRecipes]    Script Date: 11/14/2026 8:54:16 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[ManufactureSkillRecipes](
	[CharacterID] [bigint] NOT NULL,
	[SecondarySkillID] [int] NOT NULL,
	[ItemSerial] [bigint] NOT NULL,
 CONSTRAINT [PK_ManufactureSkillRecipes] PRIMARY KEY CLUSTERED 
(
	[CharacterID] ASC,
	[SecondarySkillID] ASC,
	[ItemSerial] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
/****** Object:  Index [IX_UN_ManufactureSkillRecipes_ItemSerial]    Script Date: 11/14/2026 8:54:37 AM ******/
CREATE UNIQUE NONCLUSTERED INDEX [IX_UN_ManufactureSkillRecipes_ItemSerial] ON [dbo].[ManufactureSkillRecipes]
(
	[ItemSerial] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
ALTER TABLE [dbo].[ManufactureSkillRecipes]  WITH CHECK ADD  CONSTRAINT [FK_SecondarySkills_ManufactureSkillRecipes_CharacterID_SecondarySkillID] FOREIGN KEY([CharacterID], [SecondarySkillID])
REFERENCES [dbo].[SecondarySkills] ([CharacterID], [SecondarySkillID])
GO
ALTER TABLE [dbo].[ManufactureSkillRecipes] CHECK CONSTRAINT [FK_SecondarySkills_ManufactureSkillRecipes_CharacterID_SecondarySkillID]
GO
