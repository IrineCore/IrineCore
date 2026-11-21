USE [dnworld]
GO
/****** Object:  Table [dbo].[BlockedCharacters]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[BlockedCharacters](
	[CharacterID] [bigint] NOT NULL,
	[BlockedCharacterID] [bigint] NOT NULL,
	[BlockReason] [int] NULL,
 CONSTRAINT [PK_BlockedCharacters] PRIMARY KEY CLUSTERED 
(
	[CharacterID] ASC,
	[BlockedCharacterID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
/****** Object:  Index [IX_NN_BlockedCharacters_BlockedCharacterID]    Script Date: 11/14/2026 8:54:37 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_BlockedCharacters_BlockedCharacterID] ON [dbo].[BlockedCharacters]
(
	[BlockedCharacterID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
