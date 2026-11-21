USE [dnworld]
GO
/****** Object:  Table [dbo].[LockedItems]    Script Date: 11/14/2026 8:54:16 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[LockedItems](
	[OwnerCharacterID] [bigint] NOT NULL,
	[ItemLocationCode] [tinyint] NOT NULL,
	[ItemLocationIndex] [tinyint] NOT NULL,
	[UnlockRequestDate] [datetime2](0) NULL,
	[LockedDate] [datetime2](0) NULL,
	[UnlockDate] [datetime2](0) NULL,
	[ItemSerial] [bigint] NOT NULL,
 CONSTRAINT [PK_LockedItems] PRIMARY KEY CLUSTERED 
(
	[OwnerCharacterID] ASC,
	[ItemLocationCode] ASC,
	[ItemLocationIndex] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
