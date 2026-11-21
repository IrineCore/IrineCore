USE [dnworld]
GO
/****** Object:  Table [dbo].[Appellations]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[Appellations](
	[CharacterID] [bigint] NOT NULL,
	[AppellationID] [int] NOT NULL,
	[ChoiceFlag] [bit] NOT NULL,
	[CoverChoiceFlag] [bit] NOT NULL,
 CONSTRAINT [PK_Appellations] PRIMARY KEY CLUSTERED 
(
	[CharacterID] ASC,
	[AppellationID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
/****** Object:  Index [IX_UNF_Appellations_CharacterID_ChoiceFlag]    Script Date: 11/14/2026 8:54:37 AM ******/
CREATE UNIQUE NONCLUSTERED INDEX [IX_UNF_Appellations_CharacterID_ChoiceFlag] ON [dbo].[Appellations]
(
	[CharacterID] ASC,
	[ChoiceFlag] ASC
)
WHERE ([ChoiceFlag]=CONVERT([bit],(1),(0)))
WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
/****** Object:  Index [IX_UNF_Appellations_CharacterID_CoverChoiceFlag]    Script Date: 11/14/2026 8:54:37 AM ******/
CREATE UNIQUE NONCLUSTERED INDEX [IX_UNF_Appellations_CharacterID_CoverChoiceFlag] ON [dbo].[Appellations]
(
	[CharacterID] ASC,
	[CoverChoiceFlag] ASC
)
WHERE ([CoverChoiceFlag]=CONVERT([bit],(1),(0)))
WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
GO
ALTER TABLE [dbo].[Appellations] ADD  CONSTRAINT [DF_Appellations_CoverChoiceFlag]  DEFAULT ((0)) FOR [CoverChoiceFlag]
GO
