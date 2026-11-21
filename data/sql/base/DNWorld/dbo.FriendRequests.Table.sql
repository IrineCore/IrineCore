USE [dnworld]
GO
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[FriendRequests](
	[CharacterID] [bigint] NOT NULL,
	[FriendCharacterID] [bigint] NOT NULL,
	[RegisterDate] [datetime2](0) NOT NULL CONSTRAINT [DF_FriendRequests_RegisterDate] DEFAULT (GETDATE()),
 CONSTRAINT [PK_FriendRequests] PRIMARY KEY CLUSTERED
(
	[CharacterID] ASC,
	[FriendCharacterID] ASC
)
)
GO
CREATE NONCLUSTERED INDEX [IX_NN_FriendRequests_FriendCharacterID] ON [dbo].[FriendRequests]
(
	[FriendCharacterID] ASC
)
GO
