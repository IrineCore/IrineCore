USE [dnworld]
GO
/****** Object:  Table [dbo].[CharacterStatRanking]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[CharacterStatRanking](
	[CharacterID] [bigint] NOT NULL,
	[CharacterName] [nvarchar](30) COLLATE Chinese_PRC_CI_AS NOT NULL,
	[CharacterClassCode] [tinyint] NOT NULL,
	[CharacterAbilityCode] [tinyint] NOT NULL,
	[AbilityPoint] [int] NOT NULL,
	[AbilityRank] [int] NOT NULL,
	[ClassAbilityRank] [int] NOT NULL,
	[CharacterLevel] [tinyint] NULL
) ON [DataFG1]
GO
/****** Object:  Index [IX_NC_CharacterStatRanking_ClassAbilityRank_AbilityRank]    Script Date: 11/14/2026 8:54:15 AM ******/
CREATE CLUSTERED INDEX [IX_NC_CharacterStatRanking_ClassAbilityRank_AbilityRank] ON [dbo].[CharacterStatRanking]
(
	[ClassAbilityRank] ASC,
	[AbilityRank] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
GO
/****** Object:  Index [PK_CharacterStatRanking]    Script Date: 11/14/2026 8:54:37 AM ******/
ALTER TABLE [dbo].[CharacterStatRanking] ADD  CONSTRAINT [PK_CharacterStatRanking] PRIMARY KEY NONCLUSTERED 
(
	[CharacterID] ASC,
	[CharacterAbilityCode] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
