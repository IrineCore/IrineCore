USE [dnworld]
GO
/****** Object:  Table [dbo].[GuildJoinCharacters]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[GuildJoinCharacters](
	[GuildID] [int] NOT NULL,
	[CharacterID] [bigint] NOT NULL,
 CONSTRAINT [PK_GuildJoinCharacters] PRIMARY KEY CLUSTERED 
(
	[GuildID] ASC,
	[CharacterID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
/****** Object:  Index [IX_NN_GuildJoinCharacters_CharacterID]    Script Date: 11/14/2026 8:54:37 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_GuildJoinCharacters_CharacterID] ON [dbo].[GuildJoinCharacters]
(
	[CharacterID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
ALTER TABLE [dbo].[GuildJoinCharacters]  WITH CHECK ADD  CONSTRAINT [FK_Guilds_GuildJoinCharacters_GuildID] FOREIGN KEY([GuildID])
REFERENCES [dbo].[Guilds] ([GuildID])
GO
ALTER TABLE [dbo].[GuildJoinCharacters] CHECK CONSTRAINT [FK_Guilds_GuildJoinCharacters_GuildID]
GO
