USE [dnstaging]
GO
/****** Object:  Table [dbo].[SS_Friends]    Script Date: 11/14/2026 9:00:09 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[SS_Friends](
	[CharacterID] [bigint] NOT NULL,
	[FriendCharacterID] [bigint] NOT NULL,
	[FriendMemo] [nvarchar](300) COLLATE Chinese_PRC_CI_AS NOT NULL,
	[RegisterDate] [datetime2](0) NOT NULL,
	[FriendGroupName] [nvarchar](50) COLLATE Chinese_PRC_CI_AS NULL,
	[FriendGroupCreateDate] [datetime2](0) NULL
) ON [PRIMARY]
GO
/****** Object:  Index [IX_NN_SS_Friends_CharacterID]    Script Date: 11/14/2026 9:01:05 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_SS_Friends_CharacterID] ON [dbo].[SS_Friends]
(
	[CharacterID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
/****** Object:  Index [IX_NN_SS_Friends_FriendCharacterID]    Script Date: 11/14/2026 9:01:05 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_SS_Friends_FriendCharacterID] ON [dbo].[SS_Friends]
(
	[FriendCharacterID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
