USE [dnmembership]
GO
/****** Object:  Table [dbo].[AccountAuthenticationDelQueue]    Script Date: 11/14/2026 8:51:03 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
SET ANSI_PADDING ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[AccountAuthenticationDelQueue]') AND type in (N'U'))
BEGIN
CREATE TABLE [dbo].[AccountAuthenticationDelQueue](
	[AccountID] [int] NOT NULL,
	[RegistedDate] [datetime2](0) NOT NULL,
	[MobileSerial] [varchar](max) COLLATE Chinese_PRC_CI_AS NULL,
	[CharacterList] [varchar](max) COLLATE Chinese_PRC_CI_AS NULL
) ON [DataFG1] TEXTIMAGE_ON [DataFG1]
END
GO
SET ANSI_PADDING OFF
GO
