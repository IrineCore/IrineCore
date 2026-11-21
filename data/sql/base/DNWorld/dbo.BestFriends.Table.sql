USE [dnworld]
GO
/****** Object:  Table [dbo].[BestFriends]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[BestFriends](
	[CharacterID] [bigint] NOT NULL,
	[BestFriendCharacterID] [bigint] NOT NULL,
	[FriendshipStartDate] [datetime2](0) NOT NULL,
	[MyMemo] [nvarchar](300) COLLATE Chinese_PRC_CI_AS NULL,
	[BestFreindStatusFlag] [bit] NOT NULL,
	[FriendshipEndFlag] [bit] NULL,
	[FriendshipEndDate] [datetime2](0) NULL,
	[ItemSerial] [bigint] NULL,
 CONSTRAINT [PK_BestFriends] PRIMARY KEY CLUSTERED 
(
	[CharacterID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
/****** Object:  Index [IX_UNI_BestFriends_BestFriendCharacterID]    Script Date: 11/14/2026 8:54:37 AM ******/
CREATE UNIQUE NONCLUSTERED INDEX [IX_UNI_BestFriends_BestFriendCharacterID] ON [dbo].[BestFriends]
(
	[BestFriendCharacterID] ASC
)
INCLUDE([MyMemo]) WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
ALTER TABLE [dbo].[BestFriends] ADD  CONSTRAINT [DF_BestFriends_FriendshipStartDate]  DEFAULT (getdate()) FOR [FriendshipStartDate]
GO
