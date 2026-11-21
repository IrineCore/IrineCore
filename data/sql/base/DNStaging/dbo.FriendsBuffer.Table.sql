USE [dnstaging]
GO
/****** Object:  Table [dbo].[FriendsBuffer]    Script Date: 11/14/2026 9:00:09 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[FriendsBuffer](
	[CharacterID] [bigint] NOT NULL,
	[FriendCharacterID] [bigint] NOT NULL,
	[FriendMemo] [nvarchar](300) COLLATE Chinese_PRC_CI_AS NOT NULL,
	[RegisterDate] [datetime2](0) NOT NULL,
	[FriendGroupName] [nvarchar](50) COLLATE Chinese_PRC_CI_AS NULL,
	[FriendGroupCreateDate] [datetime2](0) NULL
) ON [PRIMARY]
GO
