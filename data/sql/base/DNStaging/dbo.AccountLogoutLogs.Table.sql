USE [dnstaging]
GO
/****** Object:  Table [dbo].[AccountLogoutLogs]    Script Date: 11/14/2026 9:00:09 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
SET ANSI_PADDING ON
GO
CREATE TABLE [dbo].[AccountLogoutLogs](
	[LogoutDate] [datetime2](0) NOT NULL,
	[AccountID] [int] NOT NULL,
	[LoginDate] [datetime2](0) NOT NULL,
	[LoginIP] [int] NOT NULL,
	[SessionID] [int] NOT NULL,
	[MachineID] [binary](16) NULL
) ON [PRIMARY]
GO
SET ANSI_PADDING OFF
GO
